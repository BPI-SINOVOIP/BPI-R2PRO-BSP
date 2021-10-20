import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { OsdComponent } from './osd.component';

describe('OsdComponent', () => {
  let component: OsdComponent;
  let fixture: ComponentFixture<OsdComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ OsdComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(OsdComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
