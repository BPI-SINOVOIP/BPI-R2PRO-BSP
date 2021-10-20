import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ProtocolComponent } from './protocol.component';

describe('ProtocolComponent', () => {
  let component: ProtocolComponent;
  let fixture: ComponentFixture<ProtocolComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ProtocolComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ProtocolComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
