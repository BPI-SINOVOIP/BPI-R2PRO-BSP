import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { ConfigEventComponent } from './config-event.component';

describe('ConfigEventComponent', () => {
  let component: ConfigEventComponent;
  let fixture: ComponentFixture<ConfigEventComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ ConfigEventComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ConfigEventComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
